runner()
{
  CMD="$*"
  if command -v sudo >/dev/null 2>&1; then
    echo "[runner] 'sudo' found! Running: sudo $CMD"
    sudo bash -c "$CMD"
  else
    echo "[runner] 'sudo' not found! Running: $CMD"
    bash -c "$CMD"
  fi
}

gh_try() 
{
  if ! command -v gh >/dev/null 2>&1; then
    echo "'gh' (GitHub CLI) not found. Installing gh..."
    apt-get install -y gnupg >/dev/null 2>&1
    apt-get install -y curl  >/dev/null 2>&1
    
    curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | \
      gpg --dearmor -o /usr/share/keyrings/githubcli-archive-keyring.gpg 
      
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | \
      tee /etc/apt/sources.list.d/github-cli.list > /dev/null

    apt-get update >/dev/null 2>&1  
    apt-get install gh -y >/dev/null 2>&1 
  else
    echo "Running gh commands..."
    gh auth status || true
    gh repo view || true
  fi
}


release_with_artifact() 
{
  #         release_with_artifact '${{ github.repository }}' '${{ github.actor }}' XYZ text.txt 
  
  repo=$1
  actor=$2
  tag_name=$3
  artifact=$4  
  
  #actor="${{ github.actor }}"
  #repo="${{ github.repository }}"

  git config user.name "$actor"
  git config user.email "$actor+$actor@users.noreply.github.com"

  gh release delete "$tag_name" --repo "$repo" --yes 2>/dev/null || echo "No existing release to delete."        
  git fetch --tags
  git push --delete origin "$tag_name" 2>/dev/null || echo "No remote tag to delete."
  git tag --delete "$tag_name" 2>/dev/null || echo "No local tag to delete."

  gh release create "$tag_name" \
    --repo "$repo" \
    --title "Project: '${tag_name}' " \
    --notes "Released at '$(date)' by '$actor'"

  sleep 5
  ls -la $artifact 
  gh release upload "$tag_name" "$artifact" --repo "$repo"

  echo "gh release upload $tag_name $artifact --repo $repo, Done!"
}


key_create()
{
  keytool -genkey -v -storetype JKS -keyalg RSA -keysize 2048 -validity 10000 \
  -storepass KEYSTORE_PASSWORD \
  -keypass KEY_PASSWORD \
  -alias KEY_ALIAS \
  -keystore keystore.jks \
  -dname "CN=${1},OU=,O=,L=,S=,C=US"

  echo '{
    "android": {
      "keystore": {
        "keystorePath": "keystore.jks",
        "keystorePassword": "KEYSTORE_PASSWORD",
        "keyAlias": "KEY_ALIAS",
        "keyPassword": "KEY_PASSWORD"
      }
    }
  }' > credentials.json

  keytool -list -v -keystore keystore.jks -alias KEY_ALIAS -storepass KEYSTORE_PASSWORD 

  cat credentials.json 
}
